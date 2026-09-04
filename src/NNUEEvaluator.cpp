#include <Evaluator.hpp>

#include <iostream>


LiquorChess::NNUEEvaluator::NNUEEvaluator()
{
    if (std::filesystem::exists("liquor-eval.nnue"))
    {
        MantaRay::BinaryFileStream stream{ "liquor-eval.nnue" };
        network = NeuralNetwork{stream};
    } else
    {
        MantaRay::MarlinflowStream stream{"liquor-eval.json"};
        network = NeuralNetwork{stream};
    }
}

LiquorChess::NNUEEvaluator::~NNUEEvaluator()
{
    MantaRay::BinaryFileStream stream{ "liquor-eval.nnue" };
    network.WriteTo(stream);
}

LiquorChess::Centipawn LiquorChess::NNUEEvaluator::Evaluate(const chess::Board& board)
{
    return network.Evaluate(board.sideToMove());
}

void LiquorChess::NNUEEvaluator::Update(const chess::Board& board)
{
    network.RefreshAccumulator();

    for (int32_t i = 0; i < 64; ++i)
    {
        chess::Piece piece = board.at(chess::Square{ i });
        if (piece == chess::Piece::NONE)
            continue;

        network.EfficientlyUpdateAccumulator<MantaRay::AccumulatorOperation::Activate>(
            piece.type(), piece.color(), i);
    }
}

void LiquorChess::NNUEEvaluator::MakeMove(const chess::Board& board, const chess::Move& move)
{
    network.PushAccumulator();

    switch (move.typeOf())
    {
    case chess::Move::ENPASSANT:
        {
            network.EfficientlyUpdateAccumulator<MantaRay::AccumulatorOperation::Deactivate>(
                    static_cast<uint8_t>(chess::PieceType::PAWN), ~board.sideToMove(), move.to().ep_square().index());
            network.EfficientlyUpdateAccumulator(
                board.at<chess::PieceType>(move.from()),
                board.sideToMove(),
                   move.from().index(),
                     move.to().index());
            break;
        }
    case chess::Move::CASTLING:
        {
            assert(board.at<chess::PieceType>(move.from()) == chess::PieceType::KING);
            assert(board.at<chess::PieceType>(move.to()) == chess::PieceType::ROOK);

            const bool kindSide = move.to() > move.from();
            const chess::Square rookTo = chess::Square::castling_rook_square(kindSide, board.sideToMove());
            const chess::Square kingTo = chess::Square::castling_king_square(kindSide, board.sideToMove());

            network.EfficientlyUpdateAccumulator(
                board.at<chess::PieceType>(move.to()),
                board.sideToMove(),
                   move.to().index(),
                     rookTo.index());
            network.EfficientlyUpdateAccumulator(
                board.at<chess::PieceType>(move.from()),
                board.sideToMove(),
                   move.from().index(),
                     kingTo.index());
            break;
        }
    case chess::Move::PROMOTION:
        {
            if (board.isCapture(move))
            {
                network.EfficientlyUpdateAccumulator<MantaRay::AccumulatorOperation::Deactivate>(
                        board.at<chess::PieceType>(move.to()), ~board.sideToMove(), move.to().index());
            }
            network.EfficientlyUpdateAccumulator<MantaRay::AccumulatorOperation::Deactivate>(
                    board.at<chess::PieceType>(move.from()), board.sideToMove(), move.from().index());
            network.EfficientlyUpdateAccumulator<MantaRay::AccumulatorOperation::Activate>(
                    move.promotionType(), board.sideToMove(), move.to().index());
            break;
        }
    default:
        {
            if (board.isCapture(move))
            {
                network.EfficientlyUpdateAccumulator<MantaRay::AccumulatorOperation::Deactivate>(
                    board.at<chess::PieceType>(move.to()), ~board.sideToMove(), move.to().index());
            }
            network.EfficientlyUpdateAccumulator(
                board.at<chess::PieceType>(move.from()),
                board.sideToMove(),
                   move.from().index(),
                     move.to().index());
            break;
        }
    }
}

void LiquorChess::NNUEEvaluator::UnmakeMove(const chess::Board& board, const chess::Move& move)
{
    network.PullAccumulator();
}